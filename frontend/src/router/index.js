import { createRouter, createWebHistory } from 'vue-router'
import { isAuthenticated } from '@/auth'
import WelcomeView from '@/views/WelcomeView.vue'
import HomeView from '@/views/HomeView.vue'
import CreateRoomView from '@/views/actions/CreateRoomView.vue'
import JoinRoomView from '@/views/actions/JoinRoomView.vue'
import LeaveRoomView from '@/views/actions/LeaveRoomView.vue'
import DeleteRoomView from '@/views/actions/DeleteRoomView.vue'
import RoomOptionsView from '@/views/RoomOptionsView.vue'
import RemoveUserView from '@/views/actions/RemoveUserView.vue'
import PromoteUserView from '@/views/actions/PromoteUserView.vue'
import DemoteAdminView from '@/views/actions/DemoteAdminView.vue'

const routes = [
    { path: '/', component: WelcomeView },
    { path: '/home', component: HomeView, meta: { requiresAuth: true } },
    { path: '/create-room', component: CreateRoomView, meta: { requiresAuth: true } },
    { path: '/join-room', component: JoinRoomView, meta: { requiresAuth: true } },
    { path: '/leave-room', component: LeaveRoomView, meta: { requiresAuth: true } },
    { path: '/delete-room', component: DeleteRoomView, meta: { requiresAuth: true } },
    { path: '/room-options', component: RoomOptionsView, meta: { requiresAuth: true } },
    { path: '/remove-user', component: RemoveUserView, meta: { requiresAuth: true } },
    { path: '/promote-user', component: PromoteUserView, meta: { requiresAuth: true } },
    { path: '/demote-admin', component: DemoteAdminView, meta: { requiresAuth: true } },
]

const router = createRouter ({
    history: createWebHistory(import.meta.env.BASE_URL),
    routes,
})

router.beforeEach((to, from, next) => {
    if (to.meta.requiresAuth && !isAuthenticated.value) {
        // Verify token?
        next('/')
    }
    else {
        next()
    }
})

export default router
