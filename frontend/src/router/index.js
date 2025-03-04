import { createRouter, createWebHistory } from 'vue-router'
import { isAuthenticated } from '@/auth'
import WelcomeView from '../views/WelcomeView.vue'
import HomeView from '../views/HomeView.vue'
import AboutView from '../views/AboutView.vue'

const routes = [
    { path: '/', component: WelcomeView },
    { path: '/home', component: HomeView, meta: { requiresAuth: true } },
]

const router = createRouter ({
    history: createWebHistory(import.meta.env.BASE_URL),
    routes,
})

router.beforeEach((to, from, next) => {
    if (to.meta.requiresAuth && !isAuthenticated) {
        next('/')
    }
    else {
        next()
    }
})

export default router
